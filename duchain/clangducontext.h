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

#ifndef CLANGDUCONTEXT_H
#define CLANGDUCONTEXT_H

#include <language/duchain/ducontext.h>
#include <language/duchain/topducontext.h>

template<class BaseContext, int IdentityT>
class ClangDUContext : public BaseContext
{
public:
    template<class Data>
    ClangDUContext(Data& data) : BaseContext(data) {
    }

    ///Parameters will be reached to the base-class
    template<typename... Params>
    ClangDUContext(Params... params) : BaseContext(params...) {
        static_cast<KDevelop::DUChainBase*>(this)->d_func_dynamic()->setClassId(this);
    }

    virtual QWidget* createNavigationWidget(KDevelop::Declaration* decl = 0, KDevelop::TopDUContext* topContext = 0,
                                            const QString& htmlPrefix = QString(), const QString& htmlSuffix = QString()) const override;

    enum {
        Identity = IdentityT
    };

    static void registerItem();
    static void unregisterItem();
};

using ClangTopDUContext = ClangDUContext<KDevelop::TopDUContext, 140>;
using ClangNormalDUContext = ClangDUContext<KDevelop::DUContext, 141>;

#endif // CLANGDUCONTEXT_H
