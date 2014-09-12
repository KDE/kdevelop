/*
 * This file is part of KDevelop
 * Copyright 2014 Milian Wolff <mail@milianw.de>
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "navigationwidget.h"

#include "macronavigationcontext.h"

#include <language/duchain/navigation/abstractdeclarationnavigationcontext.h>
#include <language/duchain/navigation/abstractincludenavigationcontext.h>
#include <language/util/includeitem.h>

using namespace KDevelop;

class DeclarationNavigationContext : public AbstractDeclarationNavigationContext
{
public:
    DeclarationNavigationContext(const DeclarationPointer& decl, AbstractNavigationContext* previousContext = 0)
        : AbstractDeclarationNavigationContext(decl, {}, previousContext)
    {
    }
};

class IncludeNavigationContext : public KDevelop::AbstractIncludeNavigationContext
{
public:
    IncludeNavigationContext(const KDevelop::IncludeItem& item, KDevelop::TopDUContextPointer topContext);

protected:
    virtual bool filterDeclaration(KDevelop::Declaration* decl);
};

IncludeNavigationContext::IncludeNavigationContext(const IncludeItem& item, KDevelop::TopDUContextPointer topContext)
    : AbstractIncludeNavigationContext(item, topContext, StandardParsingEnvironment)
{}

bool IncludeNavigationContext::filterDeclaration(Declaration* decl)
{
    QString declId = decl->identifier().identifier().str();
    //filter out forward-declarations and macro-expansions without a range
    //And filter out declarations with reserved identifiers
    return !decl->qualifiedIdentifier().toString().isEmpty() && !decl->range().isEmpty() && !decl->isForwardDeclaration()
            && !(declId.startsWith("__") || (declId.startsWith("_") && declId.length() > 1 && declId[1].isUpper()) );
}

ClangNavigationWidget::ClangNavigationWidget(const DeclarationPointer& declaration)
    : AbstractNavigationWidget()
{
    if (auto macro = declaration.dynamicCast<MacroDefinition>()) {
        initBrowser(200);

        //The first context is registered so it is kept alive by the shared-pointer mechanism
        m_startContext = NavigationContextPointer(new MacroNavigationContext(macro));
        setContext( m_startContext );
    } else {
        initBrowser(400);

        //The first context is registered so it is kept alive by the shared-pointer mechanism
        m_startContext = NavigationContextPointer(new DeclarationNavigationContext(declaration));
        setContext( m_startContext );
    }
}

ClangNavigationWidget::ClangNavigationWidget(const MacroDefinition::Ptr& macro, const KDevelop::DocumentCursor& expansionLocation)
{
    initBrowser(400);

    //The first context is registered so it is kept alive by the shared-pointer mechanism
    m_startContext = NavigationContextPointer(new MacroNavigationContext(macro, expansionLocation));
    setContext(m_startContext);
}

ClangNavigationWidget::ClangNavigationWidget(const IncludeItem& includeItem, KDevelop::TopDUContextPointer topContext,
                                             const QString& htmlPrefix, const QString& htmlSuffix)
{
    m_topContext = topContext;

    initBrowser(200);

    //The first context is registered so it is kept alive by the shared-pointer mechanism
    m_startContext = NavigationContextPointer(new IncludeNavigationContext(includeItem, m_topContext));
    m_startContext->setPrefixSuffix( htmlPrefix, htmlSuffix );
    setContext( m_startContext );
}

QString ClangNavigationWidget::shortDescription(const IncludeItem& includeItem) {
  NavigationContextPointer ctx(new IncludeNavigationContext(includeItem, {}));
  return ctx->html(true);
}
