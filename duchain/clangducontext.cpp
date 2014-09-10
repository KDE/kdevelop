/*
 * Copyright 2014  Kevin Funk <kfunk@kde.org>
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

#include "clangducontext.h"

#include "duchain/navigationwidget.h"
#include "../util/clangdebug.h"

#include <language/duchain/duchainregister.h>
#include <language/duchain/topducontextdata.h>
#include <language/util/includeitem.h>

using namespace KDevelop;

REGISTER_DUCHAIN_ITEM_WITH_DATA(ClangTopDUContext, TopDUContextData);
REGISTER_DUCHAIN_ITEM_WITH_DATA(ClangNormalDUContext, DUContextData);

template<>
QWidget* ClangTopDUContext::createNavigationWidget(Declaration* decl, TopDUContext* topContext,
                                                   const QString& htmlPrefix, const QString& htmlSuffix) const
{
    if (!decl) {
        KUrl u(url().str());
        IncludeItem item;
        item.pathNumber = -1;
        item.name = u.fileName();
        item.isDirectory = false;
        item.basePath = u.upUrl();

        return new ClangNavigationWidget(item, TopDUContextPointer(topContext ? topContext : this->topContext()), htmlPrefix, htmlSuffix);
    }
    return new ClangNavigationWidget(DeclarationPointer(decl));
}

template<>
QWidget* ClangNormalDUContext::createNavigationWidget(Declaration* decl, TopDUContext* /*topContext*/,
                                                      const QString& /*htmlPrefix*/, const QString& /*htmlSuffix*/) const
{

    if (!decl) {
        clangDebug() << "no declaration, not returning navigationwidget";
        return 0;
    }
    return new ClangNavigationWidget(DeclarationPointer(decl));
}
