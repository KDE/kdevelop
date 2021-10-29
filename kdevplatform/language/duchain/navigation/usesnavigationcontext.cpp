/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "usesnavigationcontext.h"

#include "useswidget.h"
#include <KLocalizedString>
#include <language/duchain/declaration.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>

using namespace KDevelop;

UsesNavigationContext::UsesNavigationContext(IndexedDeclaration declaration, AbstractNavigationContext* previousContext)
    : AbstractNavigationContext(TopDUContextPointer(), previousContext)
    , m_declaration(declaration)
{
    m_widget = new UsesWidget(m_declaration);
}

UsesNavigationContext::~UsesNavigationContext()
{
    delete m_widget;
}

QString UsesNavigationContext::name() const
{
    return QStringLiteral("Uses");
}

QString UsesNavigationContext::html(bool shorten)
{
    Q_UNUSED(shorten);
    clear();
    modifyHtml()  += QStringLiteral("<html><body><p>");

    if (auto context = previousContext()) {
        modifyHtml() += navigationHighlight(i18n("Uses of "));
        makeLink(context->name(), context->name(), NavigationAction(context));
    } else {
        KDevelop::DUChainReadLocker lock(DUChain::lock());
        if (Declaration* decl = m_declaration.data()) {
            makeLink(i18n("Uses of %1", decl->toString()), DeclarationPointer(
                         decl), NavigationAction::NavigateDeclaration);
        }
    }

    modifyHtml() += QStringLiteral("</p></body></html>");

    return currentHtml();
}

QWidget* UsesNavigationContext::widget() const
{
    return m_widget;
}
