/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clangducontext.h"

#include "duchain/navigationwidget.h"
#include "../util/clangdebug.h"

#include <language/duchain/topducontextdata.h>
#include <language/util/includeitem.h>

using namespace KDevelop;

template <>
KDevelop::AbstractNavigationWidget*
ClangTopDUContext::createNavigationWidget(Declaration* decl, TopDUContext* topContext,
                                          KDevelop::AbstractNavigationWidget::DisplayHints hints) const
{
    if (!decl) {
        const QUrl u = url().toUrl();
        IncludeItem item;
        item.pathNumber = -1;
        item.name = u.fileName();
        item.isDirectory = false;
        item.basePath = u.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash);

        return new ClangNavigationWidget(item, TopDUContextPointer(topContext ? topContext : this->topContext()), hints);
    }
    return new ClangNavigationWidget(DeclarationPointer(decl), hints);
}

template <>
KDevelop::AbstractNavigationWidget*
ClangNormalDUContext::createNavigationWidget(Declaration* decl, TopDUContext* /*topContext*/,
                                             KDevelop::AbstractNavigationWidget::DisplayHints hints) const
{
    if (!decl) {
        clangDebug() << "no declaration, not returning navigationwidget";
        return nullptr;
    }
    return new ClangNavigationWidget(DeclarationPointer(decl), hints);
}

DUCHAIN_DEFINE_TYPE_WITH_DATA(ClangNormalDUContext, DUContextData)
DUCHAIN_DEFINE_TYPE_WITH_DATA(ClangTopDUContext, TopDUContextData)
