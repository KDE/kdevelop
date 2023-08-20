/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "navigationwidget.h"

#include "macronavigationcontext.h"
#include "types/classspecializationtype.h"

#include <language/duchain/navigation/abstractdeclarationnavigationcontext.h>
#include <language/duchain/navigation/abstractincludenavigationcontext.h>
#include <language/util/includeitem.h>

using namespace KDevelop;

class DeclarationNavigationContext : public AbstractDeclarationNavigationContext
{
    Q_OBJECT
public:
    using AbstractDeclarationNavigationContext::AbstractDeclarationNavigationContext;

    void htmlIdentifiedType(AbstractType::Ptr type, const IdentifiedType* idType) override
    {
        AbstractDeclarationNavigationContext::htmlIdentifiedType(type, idType);

        if (auto cst = dynamic_cast<const ClassSpecializationType*>(type.data())) {
            modifyHtml() += QStringLiteral("< ").toHtmlEscaped();

            bool first = true;
            const auto& templateParameters = cst->templateParameters();
            for (const auto& type : templateParameters) {
                if (first) {
                    first = false;
                } else {
                    modifyHtml() += QStringLiteral(", ");
                }

                eventuallyMakeTypeLinks(type.abstractType());
            }

            modifyHtml() += QStringLiteral(" >").toHtmlEscaped();
        }
    }
};

class IncludeNavigationContext : public KDevelop::AbstractIncludeNavigationContext
{
    Q_OBJECT
public:
    IncludeNavigationContext(const KDevelop::IncludeItem& item, const KDevelop::TopDUContextPointer& topContext);

protected:
    bool filterDeclaration(KDevelop::Declaration* decl) override;
};

IncludeNavigationContext::IncludeNavigationContext(const IncludeItem& item, const KDevelop::TopDUContextPointer& topContext)
    : AbstractIncludeNavigationContext(item, topContext, StandardParsingEnvironment)
{}

bool IncludeNavigationContext::filterDeclaration(Declaration* decl)
{
    //filter out forward-declarations and macro-expansions without a range
    //And filter out declarations with reserved identifiers
    return !decl->qualifiedIdentifier().toString().isEmpty() && !decl->range().isEmpty()
        && !decl->isForwardDeclaration() && !decl->identifier().isReserved();
}

ClangNavigationWidget::ClangNavigationWidget(const DeclarationPointer& declaration, KDevelop::AbstractNavigationWidget::DisplayHints hints)
    : AbstractNavigationWidget()
{
    setDisplayHints(hints);
    if (auto macro = declaration.dynamicCast<MacroDefinition>()) {
        initBrowser(200);

        setContext(NavigationContextPointer(new MacroNavigationContext(macro)));
    } else {
        initBrowser(400);

        setContext(NavigationContextPointer(new DeclarationNavigationContext(declaration, {})));
    }
}

ClangNavigationWidget::ClangNavigationWidget(const MacroDefinition::Ptr& macro, const KDevelop::DocumentCursor& expansionLocation,
                                             KDevelop::AbstractNavigationWidget::DisplayHints hints)
    : AbstractNavigationWidget()
{
    setDisplayHints(hints);
    initBrowser(400);

    setContext(NavigationContextPointer(new MacroNavigationContext(macro, expansionLocation)));
}

ClangNavigationWidget::ClangNavigationWidget(const IncludeItem& includeItem, const KDevelop::TopDUContextPointer& topContext,
                                             KDevelop::AbstractNavigationWidget::DisplayHints hints)
    : AbstractNavigationWidget()
{
    setDisplayHints(hints);
    initBrowser(200);

    //The first context is registered so it is kept alive by the shared-pointer mechanism
    auto context = new IncludeNavigationContext(includeItem, topContext);
    setContext(NavigationContextPointer(context));
}

QString ClangNavigationWidget::shortDescription(const IncludeItem& includeItem)
{
  IncludeNavigationContext ctx(includeItem, {});
  return ctx.html(true);
}

#include "navigationwidget.moc"
#include "moc_navigationwidget.cpp"
