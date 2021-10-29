/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@googlemail.com>
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "qmljsducontext.h"
#include "debug.h"

#include <language/duchain/topducontext.h>
#include <language/duchain/topducontextdata.h>
#include <language/duchain/duchainregister.h>
#include <language/duchain/duchainpointer.h>
#include <language/util/includeitem.h>

#include "navigation/navigationwidget.h"

using namespace KDevelop;

namespace QmlJS {

template <>
AbstractNavigationWidget* QmlJSTopDUContext::createNavigationWidget(Declaration* decl, TopDUContext* topContext,
                                                                    AbstractNavigationWidget::DisplayHints hints) const
{
    if (!decl) {
        const QUrl u = url().toUrl();
        IncludeItem item;
        item.pathNumber = -1;
        item.name = u.fileName();
        item.isDirectory = false;
        item.basePath = u.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash);

        return new NavigationWidget(item, TopDUContextPointer(topContext ? topContext : this->topContext()), hints);
    }
    return new NavigationWidget(decl, topContext, hints);
}

template <>
AbstractNavigationWidget*
QmlJSNormalDUContext::createNavigationWidget(Declaration* decl, TopDUContext* topContext,
                                             AbstractNavigationWidget::DisplayHints hints) const
{
    if (!decl) {
        qCDebug(KDEV_QMLJS_DUCHAIN) << "no declaration, not returning navigationwidget";
        return nullptr;
    }
    return new NavigationWidget(decl, topContext, hints);
}
}

DUCHAIN_DEFINE_TYPE_WITH_DATA(QmlJS::QmlJSNormalDUContext, DUContextData)
DUCHAIN_DEFINE_TYPE_WITH_DATA(QmlJS::QmlJSTopDUContext, TopDUContextData)
