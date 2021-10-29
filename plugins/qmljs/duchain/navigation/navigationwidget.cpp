/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "navigationwidget.h"
#include "declarationnavigationcontext.h"

#include <language/duchain/topducontext.h>
#include <language/duchain/navigation/abstractincludenavigationcontext.h>

using namespace KDevelop;

namespace QmlJS {

NavigationWidget::NavigationWidget(KDevelop::Declaration* decl,
                                   KDevelop::TopDUContext* topContext,
                                   KDevelop::AbstractNavigationWidget::DisplayHints hints)
{
    auto context = new DeclarationNavigationContext(
        DeclarationPointer(decl),
        TopDUContextPointer(topContext)
    );
    setContext(NavigationContextPointer(context));
    setDisplayHints(hints);
}

NavigationWidget::NavigationWidget(const KDevelop::IncludeItem& includeItem,
                                   const KDevelop::TopDUContextPointer& topContext,
                                   KDevelop::AbstractNavigationWidget::DisplayHints hints)
    : AbstractNavigationWidget()
{
    setDisplayHints(hints);
    initBrowser(200);

    auto context = new AbstractIncludeNavigationContext(
        includeItem, topContext,
        StandardParsingEnvironment);
    setContext(NavigationContextPointer(context));
}
}
