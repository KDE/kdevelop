/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGDUCONTEXT_H
#define CLANGDUCONTEXT_H

#include <language/duchain/duchainregister.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/topducontext.h>

template<class BaseContext, int IdentityT>
class ClangDUContext : public BaseContext
{
public:
    template<class Data>
    explicit ClangDUContext(Data& data) : BaseContext(data) {
    }

    ///Parameters will be reached to the base-class
    template<typename... Params>
    explicit ClangDUContext(Params... params) : BaseContext(params...) {
        static_cast<KDevelop::DUChainBase*>(this)->d_func_dynamic()->setClassId(this);
    }

    KDevelop::AbstractNavigationWidget*
    createNavigationWidget(KDevelop::Declaration* decl = nullptr, KDevelop::TopDUContext* topContext = nullptr,
                           KDevelop::AbstractNavigationWidget::DisplayHints hints
                           = KDevelop::AbstractNavigationWidget::NoHints) const override;

    enum {
        Identity = IdentityT
    };
};

using ClangTopDUContext = ClangDUContext<KDevelop::TopDUContext, 140>;
using ClangNormalDUContext = ClangDUContext<KDevelop::DUContext, 141>;

DUCHAIN_DECLARE_TYPE(ClangTopDUContext)
DUCHAIN_DECLARE_TYPE(ClangNormalDUContext)

#endif // CLANGDUCONTEXT_H
